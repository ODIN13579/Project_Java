package com.swiftletcare.security;

import com.swiftletcare.api.ApiError;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.security.oauth2.jwt.Jwt;
import org.springframework.stereotype.Service;

@Service
public class Access {
  private final JdbcTemplate jdbc;

  public Access(JdbcTemplate jdbc) {
    this.jdbc = jdbc;
  }

  public boolean canRead(Jwt jwt, long device) {
    if (jwt == null
        || jwt.getExpiresAt() == null
        || !jwt.getExpiresAt().isAfter(java.time.Instant.now())) return false;
    return Boolean.TRUE.equals(
        jdbc.queryForObject(
            """
select exists(select 1 from swiftletcare.devices d join swiftletcare.zones z on z.id=d.zone_id
join swiftletcare.houses h on h.id=z.house_id join swiftletcare.house_memberships m on m.house_id=h.id
join swiftletcare.users u on u.id=m.user_id where d.id=? and u.id=? and u.active and m.active and z.active and h.active)
""",
            Boolean.class,
            device,
            Long.valueOf(jwt.getSubject())));
  }

  public void require(Jwt jwt, long device) {
    if (!canRead(jwt, device))
      throw new ApiError.Failure(403, "ACCESS_DENIED", "No active house membership");
  }
}
